module MitamaeSecrets
  class Store
    def initialize(base_dir)
      @base_dir = base_dir
    end

    DEFAULT = Object.new

    def fetch(name, default = DEFAULT)
      path = values_path(name)
      if File.exist?(path)
        decrypt(path)
      else
        if default.equal?(DEFAULT)
          raise KeyError.new("Unknown secret variable: #{name}")
        else
          default
        end
      end
    end

    private

    def values_path(name)
      File.join(@base_dir, 'values', name)
    end

    def keychain
      @keychain ||= Keychain.new(keychain_path)
    end

    def keychain_path
      File.join(@base_dir, 'keys')
    end

    def decrypt(path)
      json = JSON.parse(File.read(path))
      version = json.delete('version')
      algorithm = json.delete('algorithm')
      ciphertext = json.delete('ciphertext')
      key_name = json.delete('key_name')
      Decryptors.create(version, algorithm, keychain.load(key_name), json).decrypt(ciphertext).fetch('value')
    end
  end
end
