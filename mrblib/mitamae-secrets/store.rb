module MitamaeSecrets
  class Store
    def initialize(base_dir)
      @base_dir = base_dir
      ensure_directories!
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

    def store(name, value, key = 'default')
      path = values_path(name)
      validate_name!(name)
      encrypted_data = Encryptors.create(1, 'aes-256-gcm', keychain.load(key)).encrypt({value: value}.to_json)

      File.open(path, 'w', 0600) do |io|
        io.puts encrypted_data.to_json
      end
    end

    private

    def ensure_directories!
      mkdir_p(@base_dir)
      mkdir_p(File.join(@base_dir, 'keys'))
      mkdir_p(File.join(@base_dir, 'values'))
    end

    def mkdir_p(path)
      # XXX: Should make all parent directories
      unless Dir.exist?(path)
        Dir.mkdir(path)
      end
    end

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

    def validate_name!(name)
      if name.include?("\\") || name.include?('/') || name.include?(':')
        raise ArgumentError.new('value name must not contain slashes, commas, backslackes')
      end
    end
  end
end
