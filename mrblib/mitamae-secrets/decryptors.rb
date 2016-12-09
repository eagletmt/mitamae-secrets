module MitamaeSecrets
  module Decryptors
    def self.create(version, algorithm, aes_key, options)
      class_for(version, algorithm).new(aes_key, options)
    end

    def self.class_for(version, algorithm)
      if version == 1 && algorithm == 'aes-256-gcm'
        Aes256GcmV1
      else
        raise "Unknown encrypted data: version=#{version} algorithm=#{algorithm}"
      end
    end
  end
end
