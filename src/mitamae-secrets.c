#include <mruby.h>
#include <mruby/array.h>
#include <mruby/string.h>
#include <mruby/variable.h>
#include <openssl/err.h>
#include <openssl/evp.h>

static mrb_value m_aes_256_gcm_decrypt(mrb_state *mrb, mrb_value self) {
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

void mrb_mitamae_secrets_gem_init(mrb_state *mrb) {
  struct RClass *module = mrb_define_module(mrb, "MitamaeSecrets");
  struct RClass *decriptors =
      mrb_define_module_under(mrb, module, "Decryptors");
  struct RClass *aes_256_gcm =
      mrb_define_class_under(mrb, decriptors, "Aes256GcmV1", mrb->object_class);

  mrb_define_method(mrb, aes_256_gcm, "decrypt", m_aes_256_gcm_decrypt,
                    MRB_ARGS_REQ(1));
}

void mrb_mitamae_secrets_gem_final(mrb_state *mrb) {
}
