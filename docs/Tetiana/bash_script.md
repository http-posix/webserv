```text
The logger writes to the file only what passes through Logger::Log.
External stderr output does not end up there. 
To intercept it at startup:
```

`./webserv config/cgi 2>&1 | tee -a run.log`


____
> awk 'NR==1 || /[.]py/' - prints the header and lines containing `.py`;  
         the brackets stop > > `awk` from matching itself  
> [.]py -  script type  
> pid - process  
> ppid - parent process  
> stat - process state  
>        Z - zombie, finished but still in table  
>        R - running; if it lasts long, maybe an infinite loop,  
>        S - sleep and wait for event,  
>        T - stopped  
> etime - process lifetime  [[dd-]hh:]mm:ss  

`ps -ax -o pid,ppid,stat,etime,%cpu,command | awk 'NR==1 || /[w]ebserv|[.]py/ || $3 ~ /^Z/'`

```bash
while true; do
  clear
  date
  ps -ax -o pid,ppid,stat,etime,%cpu,command | awk 'NR==1 || /[w]ebserv|[.]py/ || $3 ~ /^Z/'
  sleep 1
done
```

