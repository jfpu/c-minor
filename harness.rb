#!/usr/bin/env ruby

trans_dict = {
  "lex" => "scan",
  "parse" => "print",
  "typecheck" => "typecheck",
  "compile" => "codegen"
}

if ARGV.count != 1 or !trans_dict.has_key?(ARGV[0])
  warn "invalid option [lex, parse, typecheck, compile]"
  exit 1
end

case ARGV[0]
when "lex", "parse", "typecheck"
  Dir["test_#{ARGV[0]}/good*.cminor"].each do |file|
    warn "#{file} test incorrectly failed" unless system("./cminor -#{trans_dict[ARGV[0]]} #{file} >/dev/null 2>/dev/null")
  end

  Dir["test_#{ARGV[0]}/bad*.cminor"].each do |file|
    warn "#{file} test incorrectly passed" if system("./cminor -#{trans_dict[ARGV[0]]} #{file} >/dev/null 2>/dev/null")
  end

when "compile"
  Dir["test_#{ARGV[0]}/good*.cminor"].each do |file|
    warn "#{file} test incorrectly failed" unless system("./cminor -#{trans_dict[ARGV[0]]} #{file} #{file}.s >/dev/null 2>/dev/null")
    warn "#{file} assembly doesn't compile" unless system("cc #{file}.s ./library.o -o #{file}.out")
  end
end
