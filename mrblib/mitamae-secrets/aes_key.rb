module MitamaeSecrets
  class AesKey
    def self.from_json(json)
      data = JSON.parse(json)
      new(data.fetch('name'), data.fetch('type'), data.fetch('key').unpack('m')[0])
    end

    attr_reader :key

    def initialize(name, type, key)
      if name.include?("\\") || name.include?('/') || name.include?(':')
        raise ArgumentError.new('name must not contain slashes, commas, backslackes')
      end
      @name = name
      @type = type
      @key = key
    end
  end
end
