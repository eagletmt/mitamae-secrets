#include <getopt.h>
#include <mruby.h>
#include <mruby/string.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

static char *program_name;

static void print_usage() {
  puts("Usage:");
  printf("    %s set --base=BASE_DIR variable\n", program_name);
  printf("    %s get --base=BASE_DIR variable\n", program_name);
}

static mrb_value ask_secret(mrb_state *mrb, const char *prompt) {
  struct termios oldattr, newattr;
  mrb_value str;
  char buf[1024];
  ssize_t read_bytes;
  int is_tty;

  is_tty = isatty(STDIN_FILENO);
  if (is_tty) {
    if (tcgetattr(STDIN_FILENO, &oldattr) != 0) {
      perror("tcgetattr");
      return mrb_nil_value();
    }
    newattr = oldattr;
    newattr.c_lflag &= ~ECHO;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &newattr) != 0) {
      perror("tcsetattr");
      return mrb_nil_value();
    }

    printf("%s: ", prompt);
    fflush(stdout);
  }

  str = mrb_str_new(mrb, "", 0);
  while ((read_bytes = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
    mrb_str_cat(mrb, str, buf, read_bytes);
  }

  if (is_tty) {
    if (tcsetattr(STDIN_FILENO, TCSANOW, &oldattr) != 0) {
      perror("tcsetattr");
      return mrb_nil_value();
    }
  }

  if (RSTRING_PTR(str)[RSTRING_LEN(str) - 1] == '\n') {
    mrb_str_resize(mrb, str, RSTRING_LEN(str) - 1);
  }
  return str;
}

static int cmd_set(mrb_state *mrb, int argc, char *argv[]) {
  mrb_value basedir = mrb_nil_value(), name, value;

  for (;;) {
    static struct option options[] = {
        {"base", required_argument, NULL, 'b'}, {0},
    };
    int r, idx;

    r = getopt_long(argc, argv, "b:", options, &idx);
    if (r == -1) {
      break;
    }

    switch (r) {
      case 'b':
        basedir = mrb_str_new_cstr(mrb, optarg);
        break;
      case '?':
        print_usage();
        return 1;
      default:
        printf("getopt_long returned Unknown character code %d\n", r);
        return 1;
    }
  }

  if (mrb_nil_p(basedir)) {
    fprintf(stderr, "%s: --base is required\n", program_name);
    print_usage();
    return 1;
  }

  if (optind == argc) {
    fprintf(stderr, "%s: missing variable\n", program_name);
    print_usage();
    return 1;
  } else if (optind + 1 < argc) {
    fprintf(stderr, "%s: extra arguments\n", program_name);
    print_usage();
    return 1;
  }
  name = mrb_str_new_cstr(mrb, argv[optind]);
  value = ask_secret(mrb, argv[optind]);
  if (mrb->exc) {
    return 1;
  }

  mrb_funcall(
      mrb,
      mrb_obj_new(mrb, mrb_class_get_under(
                           mrb, mrb_module_get(mrb, "MitamaeSecrets"), "Store"),
                  1, &basedir),
      "store", 2, name, value);
  return 0;
}

static int cmd_get(mrb_state *mrb, int argc, char *argv[]) {
  mrb_value basedir = mrb_nil_value(), name, value;

  for (;;) {
    static struct option options[] = {
        {"base", required_argument, NULL, 'b'}, {0},
    };
    int r, idx;

    r = getopt_long(argc, argv, "b:", options, &idx);
    if (r == -1) {
      break;
    }

    switch (r) {
      case 'b':
        basedir = mrb_str_new_cstr(mrb, optarg);
        break;
      case '?':
        print_usage();
        return 1;
      default:
        printf("getopt_long returned Unknown character code %d\n", r);
        return 1;
    }
  }

  if (mrb_nil_p(basedir)) {
    fprintf(stderr, "%s: --base is required\n", program_name);
    print_usage();
    return 1;
  }

  if (optind == argc) {
    fprintf(stderr, "%s: missing variable\n", program_name);
    print_usage();
    return 1;
  } else if (optind + 1 < argc) {
    fprintf(stderr, "%s: extra arguments\n", program_name);
    print_usage();
    return 1;
  }
  name = mrb_str_new_cstr(mrb, argv[optind]);

  value = mrb_funcall(
      mrb,
      mrb_obj_new(mrb, mrb_class_get_under(
                           mrb, mrb_module_get(mrb, "MitamaeSecrets"), "Store"),
                  1, &basedir),
      "fetch", 1, name);
  if (mrb->exc) {
    return 1;
  }
  fwrite(RSTRING_PTR(value), 1, RSTRING_LEN(value), stdout);
  putchar('\n');
  return 0;
}

int main(int argc, char *argv[]) {
  mrb_state *mrb;
  int rc;
  const char *subcmd;

  program_name = argv[0];
  if (argc < 2) {
    print_usage();
    return 1;
  }

  mrb = mrb_open();
  subcmd = argv[1];
  argv[1] = program_name;
  if (strcmp(subcmd, "set") == 0) {
    rc = cmd_set(mrb, argc - 1, argv + 1);
  } else if (strcmp(subcmd, "get") == 0) {
    rc = cmd_get(mrb, argc - 1, argv + 1);
  } else {
    fprintf(stderr, "Unknown subcommand: %s\n", argv[1]);
    print_usage();
    rc = 1;
  }

  if (mrb->exc) {
    rc = 1;
    mrb_print_error(mrb);
  }
  mrb_close(mrb);

  return rc;
}
