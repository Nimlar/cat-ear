make gcc && ./emul-gcc |grep "RightAlt" |grep -E "(write)|(Attach)" >/tmp/log.txt && grep -vR "###" /tmp/log.txt > /tmp/new.txt && grep -E "^###" /tmp/log.txt | sed -e "s,^###,,"  > /tmp/old.txt
