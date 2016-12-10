def create_default_store
  MitamaeSecrets::Store.new(File.join(File.dirname(__FILE__), 'secret'))
end

assert('interoperability between #store and #fetch') do
  store = create_default_store
  aes_key = MitamaeSecrets::AesKey.generate_random('default')
  store.keychain.save(aes_key)
  store.store('foo', 'P@ssw0rd')
  assert_equal('P@ssw0rd', store.fetch('foo'))
end

assert('cannot decrypt with different key') do
  store = create_default_store
  aes_key1 = MitamaeSecrets::AesKey.generate_random('default')
  store.keychain.save(aes_key1)
  store.store('foo', 'P@ssw0rd')

  aes_key2 = MitamaeSecrets::AesKey.generate_random('default')
  store.keychain.save(aes_key2)
  assert_raise(RuntimeError) { store.fetch('foo') }
end

assert('cannot fetch unknown variable') do
  store = create_default_store
  aes_key = MitamaeSecrets::AesKey.generate_random('default')
  store.keychain.save(aes_key)

  assert_raise(RuntimeError) { store.fetch('foo') }
end
