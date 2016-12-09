module MitamaeSecrets
  class Keychain
    class KeyNotFound < StandardError; end

    def initialize(path)
      @path = path
    end

    def load(name)
      AesKey.from_json(File.read(File.join(@path, name)))
    rescue Errno::ENOENT
      raise KeyNotFound.new("Couldn't find key #{name.inspect}")
    end
  end
end
