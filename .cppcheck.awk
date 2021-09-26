BEGIN {
  print "["
}

function getoneline(fname, lineno) {
  i=0
  
  while ((getline line < fname) > 0) {
    i++
    if (i == lineno) {
      // remove stray CR char
      gsub("\r", "", line)
      
      close(fname)
      return line
    }
  }
  
  close(fname)
  return ""
}

function sha1text(text) {
  cmd = "echo '" text "' | sha1sum"
  
  if ((cmd | getline sha1) > 0) {
    sub(/ .*/, "", sha1)
  }
  else {
    print "ERROR: sha1text() failed to hash text"
    exit 1
  }
  
  close(cmd)
  return sha1
}


{
  if(NR!=1) print ","
  
  # input line format: "{file};{line};{severity};{id};{message}"
  
  contexthash = sha1text(getoneline($1, $2) $3 $4 $5)
  
  print "{\"description\":\"(" $3 ") " $5 "\", \"fingerprint\":\"" contexthash "\", \"location\": {\"path\":\"" $1 "\", \"lines\": {\"begin\":" $2 " } } }"
}

END {
  print"]"
}
