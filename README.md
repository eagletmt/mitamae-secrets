# mitamae-secrets
[itamae-secrets](https://github.com/sorah/itamae-secrets) for [mitamae](https://github.com/k0kubun/mitamae)

## Usage

```ruby
store = MitamaeSecrets::Store.new('/path/to/secret')
# Decrypt /path/to/secret/values/secret_password with the key in /path/to/secret/keys
store.fetch('secret_password')  # => "P@ssw0rd"
```

## Acknowledgment
Original implementation is written by sorah.
https://github.com/sorah/itamae-secrets
