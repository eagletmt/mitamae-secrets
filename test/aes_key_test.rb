assert('AesKey#random_bytes') do
  [1, 32, 70].each do |size|
    assert_equal(size, MitamaeSecrets::AesKey.random_bytes(size).size)
  end
end
