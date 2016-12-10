module MitamaeSecrets
  class AesKey
    def self.key_len_for_type(type)
      case type
      when 'aes1'
        cipher_key_length('aes-256-gcm')
      else
        raise ArgumentError.new("unknown type #{type.inspect}")
      end
    end

    def self.from_json(json)
      data = JSON.parse(json)
      new(data.fetch('name'), data.fetch('type'), data.fetch('key').unpack('m')[0])
    end

    def self.generate_random(name)
      key_len = key_len_for_type('aes1')
      new(name, 'aes1', random_bytes(key_len))
    end

    attr_reader :name, :key

    def initialize(name, type, key)
      if name.include?("\\") || name.include?('/') || name.include?(':')
        raise ArgumentError.new('name must not contain slashes, commas, backslackes')
      end
      @name = name
      @type = type
      @key = key
    end

    def to_json
      {
        name: @name,
        type: @type,
        key: [@key].pack('m'),
      }.to_json
    end
  end
end
