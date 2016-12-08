#include <mruby.h>

void mrb_mitamae_secrets_gem_init(mrb_state *mrb) {
  struct RClass *module = mrb_define_module(mrb, "MitamaeSecrets");
}

void mrb_mitamae_secrets_gem_final(mrb_state *mrb) {
}
