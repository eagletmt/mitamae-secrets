MRuby::Gem::Specification.new('mitamae-secrets') do |spec|
  spec.license = 'MIT'
  spec.author = ['Kohei Suzuki']
  spec.summary = 'mitamae version of itamae-secrets'
  spec.bins = ['mitamae-secrets']

  spec.linker.libraries << 'ssl' << 'crypto'

  spec.add_dependency 'mruby-dir', github: 'iij/mruby-dir'
  spec.add_dependency 'mruby-hash-ext', core: 'mruby-hash-ext'
  spec.add_dependency 'mruby-io', github: 'iij/mruby-io'
  spec.add_dependency 'mruby-iijson', github: 'iij/mruby-iijson'
  spec.add_dependency 'mruby-pack', github: 'iij/mruby-pack'
end
