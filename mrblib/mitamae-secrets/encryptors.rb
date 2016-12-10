module MitamaeSecrets
  module Encryptors
    def self.create(version, algorithm, aes_key)
      class_for(version, algorithm).new(aes_key)
    end

    def self.class_for(version, algorithm)
      if algorithm == 'aes-256-gcm' && version == 1
        Aes256GcmV1
      else
        raise "Unknown encryptor: version=#{version} algorithm=#{algorithm}"
      end
    end
  end
end
