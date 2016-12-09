module MitamaeSecrets
  module Decryptors
    class Aes256GcmV1
      def initialize(aes_key, options)
        @key = aes_key.key
        @auth_tag = options.fetch('auth_tag').unpack('m')[0]
        @iv = options.fetch('iv').unpack('m')[0]
      end
    end
  end
end
