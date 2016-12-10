# mitamae-secrets
[![Build Status](https://travis-ci.org/eagletmt/mitamae-secrets.svg?branch=master)](https://travis-ci.org/eagletmt/mitamae-secrets)

[itamae-secrets](https://github.com/sorah/itamae-secrets) for [mitamae](https://github.com/k0kubun/mitamae)

## Usage

```ruby
store = MitamaeSecrets::Store.new('/path/to/secret')
# Decrypt /path/to/secret/values/secret_password with the key in /path/to/secret/keys
store.fetch('secret_password')  # => "P@ssw0rd"

# Store encrypted value with default key
store.store('secret_password', 'P@ssw0rd')

# Store encrypted value with "foobar" key
store.store('secret_password', 'P@ssw0rd', 'foobar')
```

## CLI

```
% mitamae-secrets set --base /path/to/secret secret_password
secret_password: 
% mitamae-secrets get --base /path/to/secret secret_password
P@ssw0rd
% echo -e "hello\nworld" | mitamae-secrets set --base /path/to/secret greeting
% mitamae-secrets get --base /path/to/secret greeting
hello
world

% mitamae-secrets newkey --base /path/to/secret foo
% cat /path/to/secret/keys/foo
{"name":"foo","type":"aes1","key":"7RLkJdcH6pDEoMOjZIzTEjj6Ih0bTCYRxvw/zYknWrE=\n"}
```

## Build with mitamae

```
% git clone --recursive https://github.com/k0kubun/mitamae
% cd mitamae
% cat > build_config.rb
MRuby::Build.new do |conf|
  toolchain :gcc

  # XXX: mitamae v1.1.2 depends on k0kubun/mruby-io and mitamae-secrets depends on iij/mruby-io.
  # Force using k0kubun/mruby-io here.
  conf.gem github: 'k0kubun/mruby-io'
  conf.gem __dir__
  conf.gem github: 'eagletmt/mitamae-secrets'
end
% rake compile
```

## Acknowledgment
Original implementation is written by sorah.
https://github.com/sorah/itamae-secrets
