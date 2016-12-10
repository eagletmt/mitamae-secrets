#include <mruby.h>
#include <mruby/array.h>
#include <mruby/string.h>
#include <mruby/variable.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

static mrb_value m_decryptors_aes_256_gcm_decrypt(mrb_state *mrb,
                                                  mrb_value self) {
  mrb_value ciphertext, key, iv, auth_tag;
  char *plaintext;
  int out_len, final_len;
  EVP_CIPHER_CTX *ctx;

  mrb_get_args(mrb, "S", &ciphertext);
  ciphertext = mrb_ary_entry(
      mrb_funcall(mrb, ciphertext, "unpack", 1, mrb_str_new_cstr(mrb, "m")), 0);
  key = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@key"));
  iv = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@iv"));
  auth_tag = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@auth_tag"));

  ctx = EVP_CIPHER_CTX_new();
  if (!EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL,
                          (const unsigned char *)RSTRING_PTR(key),
                          (const unsigned char *)RSTRING_PTR(iv))) {
    long code = ERR_peek_last_error();
    EVP_CIPHER_CTX_free(ctx);
    mrb_raisef(mrb, E_RUNTIME_ERROR, "EVP_DecryptInit_ex failed: code=%S",
               mrb_fixnum_value(code));
  }
  if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, RSTRING_LEN(auth_tag),
                           RSTRING_PTR(auth_tag))) {
    long code = ERR_peek_last_error();
    EVP_CIPHER_CTX_free(ctx);
    mrb_raisef(mrb, E_RUNTIME_ERROR,
               "EVP_CIPHER_CTX_ctrl(EVP_CTRL_GCM_SET_TAG) failed: code=%S",
               mrb_fixnum_value(code));
  }
  plaintext =
      mrb_malloc(mrb, RSTRING_LEN(ciphertext) + EVP_CIPHER_CTX_block_size(ctx));
  if (!EVP_DecryptUpdate(ctx, (unsigned char *)plaintext, &out_len,
                         (const unsigned char *)RSTRING_PTR(ciphertext),
                         RSTRING_LEN(ciphertext))) {
    long code = ERR_peek_last_error();
    EVP_CIPHER_CTX_free(ctx);
    mrb_raisef(mrb, E_RUNTIME_ERROR, "EVP_DecryptUpdate failed: code=%S",
               mrb_fixnum_value(code));
  }
  if (!EVP_DecryptFinal_ex(ctx, (unsigned char *)(plaintext + out_len),
                           &final_len)) {
    long code = ERR_peek_last_error();
    EVP_CIPHER_CTX_free(ctx);
    mrb_raisef(mrb, E_RUNTIME_ERROR, "EVP_DecryptFinal_ex failed: code=%S",
               mrb_fixnum_value(code));
  }
  EVP_CIPHER_CTX_free(ctx);

  return mrb_funcall(mrb, mrb_obj_value(mrb_module_get(mrb, "JSON")), "parse",
                     1, mrb_str_new(mrb, plaintext, out_len + final_len));
}

static mrb_value generate_random_bytes(mrb_state *mrb, int size) {
  char *buf;
  int rc;

  buf = mrb_malloc(mrb, size);
  rc = RAND_bytes((unsigned char *)buf, size);
  if (rc == 0) {
    long code = ERR_peek_last_error();
    mrb_raisef(mrb, E_RUNTIME_ERROR, "RAND_bytes failed: code=%S",
               mrb_fixnum_value(code));
  } else if (rc == -1) {
    mrb_raise(mrb, E_RUNTIME_ERROR, "RAND_bytes is not supported");
  }
  return mrb_str_new(mrb, buf, size);
}

static mrb_value m_encryptors_aes_256_gcm_encrypt(mrb_state *mrb,
                                                  mrb_value self) {
  mrb_value key, plaintext, iv, ret;
  EVP_CIPHER_CTX *ctx;
  int iv_len;
  char *ciphertext, *auth_tag;
  int out_len, final_len;

  mrb_get_args(mrb, "SS", &key, &plaintext);

  ctx = EVP_CIPHER_CTX_new();
  if (!EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL)) {
    long code = ERR_peek_last_error();
    EVP_CIPHER_CTX_free(ctx);
    mrb_raisef(mrb, E_RUNTIME_ERROR, "EVP_EncryptInit_ex failed: code=%S",
               mrb_fixnum_value(code));
  }
  iv_len = EVP_CIPHER_CTX_iv_length(ctx);
  iv = generate_random_bytes(mrb, iv_len);
  if (!EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL,
                          (const unsigned char *)RSTRING_PTR(key),
                          (const unsigned char *)RSTRING_PTR(iv))) {
    long code = ERR_peek_last_error();
    EVP_CIPHER_CTX_free(ctx);
    mrb_raisef(mrb, E_RUNTIME_ERROR, "EVP_EncryptInit_ex failed: code=%S",
               mrb_fixnum_value(code));
  }
  ciphertext =
      mrb_malloc(mrb, RSTRING_LEN(plaintext) + EVP_CIPHER_CTX_block_size(ctx));
  if (!EVP_EncryptUpdate(ctx, (unsigned char *)ciphertext, &out_len,
                         (const unsigned char *)RSTRING_PTR(plaintext),
                         RSTRING_LEN(plaintext))) {
    long code = ERR_peek_last_error();
    EVP_CIPHER_CTX_free(ctx);
    mrb_raisef(mrb, E_RUNTIME_ERROR, "EVP_EncryptUpdate failed: code=%S",
               mrb_fixnum_value(code));
  }
  if (!EVP_EncryptFinal_ex(ctx, (unsigned char *)(ciphertext + out_len),
                           &final_len)) {
    long code = ERR_peek_last_error();
    EVP_CIPHER_CTX_free(ctx);
    mrb_raisef(mrb, E_RUNTIME_ERROR, "EVP_EncryptUpdate failed: code=%S",
               mrb_fixnum_value(code));
  }
  auth_tag = mrb_malloc(mrb, 16);
  if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, auth_tag)) {
    long code = ERR_peek_last_error();
    EVP_CIPHER_CTX_free(ctx);
    mrb_raisef(mrb, E_RUNTIME_ERROR,
               "EVP_CIPHER_CTX_ctrl(EVP_CTRL_GCM_GET_TAG) failed: code=%S",
               mrb_fixnum_value(code));
  }
  EVP_CIPHER_CTX_free(ctx);

  ret = mrb_ary_new_capa(mrb, 3);
  mrb_ary_set(mrb, ret, 0, mrb_str_new(mrb, ciphertext, out_len + final_len));
  mrb_ary_set(mrb, ret, 1, iv);
  mrb_ary_set(mrb, ret, 2, mrb_str_new(mrb, auth_tag, 16));
  return ret;
}

void mrb_mitamae_secrets_gem_init(mrb_state *mrb) {
  struct RClass *module = mrb_define_module(mrb, "MitamaeSecrets");
  struct RClass *decriptors =
      mrb_define_module_under(mrb, module, "Decryptors");
  struct RClass *encryptors =
      mrb_define_module_under(mrb, module, "Encryptors");
  struct RClass *decryptors_aes_256_gcm =
      mrb_define_class_under(mrb, decriptors, "Aes256GcmV1", mrb->object_class);
  struct RClass *encryptors_aes_256_gcm =
      mrb_define_class_under(mrb, encryptors, "Aes256GcmV1", mrb->object_class);

  mrb_define_method(mrb, decryptors_aes_256_gcm, "decrypt",
                    m_decryptors_aes_256_gcm_decrypt, MRB_ARGS_REQ(1));

  mrb_define_method(mrb, encryptors_aes_256_gcm, "_encrypt",
                    m_encryptors_aes_256_gcm_encrypt, MRB_ARGS_REQ(2));
}

void mrb_mitamae_secrets_gem_final(mrb_state *mrb) {
}
