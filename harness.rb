#!/usr/bin/env ruby

if ARGV.count != 1
  warn "invalid option [lex, parse, typecheck]"
  exit 1
end

trans_dict = {
  "lex" => "scan",
  "parse" => "print",
  "typecheck" => "typecheck"
}

Dir["test_#{ARGV[0]}/good*.cminor"].each do |file|
  warn "#{file} test incorrectly failed" unless system("./cminor -#{trans_dict[ARGV[0]]} #{file} >/dev/null 2>/dev/null")
end

Dir["test_#{ARGV[0]}/bad*.cminor"].each do |file|
  warn "#{file} test incorrectly passed" if system("./cminor -#{trans_dict[ARGV[0]]} #{file} >/dev/null 2>/dev/null")
end
