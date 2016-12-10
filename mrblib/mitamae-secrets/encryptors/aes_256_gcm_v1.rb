module MitamaeSecrets
  module Encryptors
    class Aes256GcmV1
      VERSION = 1
      ALGORITHM = 'aes-256-gcm'

      def initialize(aes_key)
        @aes_key = aes_key
      end

      def encrypt(plaintext)
        ciphertext, iv, auth_tag = _encrypt(@aes_key.key, plaintext)
        {
          version: VERSION,
          algorithm: ALGORITHM,
          key_name: @aes_key.name,
          ciphertext: [ciphertext].pack('m'),
          iv: [iv].pack('m'),
          auth_tag: [auth_tag].pack('m'),
        }
      end
    end
  end
end
