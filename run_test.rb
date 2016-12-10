if __FILE__ == $0
  build_args = ARGV
  if build_args.empty?
    build_args = ['test']
  end

  repo = 'https://github.com/mruby/mruby'
  dir = 'mruby'
  unless File.exist?(dir)
    system('git', 'clone', repo, dir)
  end

  ENV['MRUBY_CONFIG'] = File.expand_path(__FILE__)
  Dir.chdir(dir)
  exec('ruby', 'minirake', *build_args)
end

MRuby::Build.new do |conf|
  toolchain :gcc
  conf.gem __dir__
  conf.enable_test
end
