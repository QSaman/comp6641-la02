 #!/bin/bash

 echo -e "GET / HTTP/1.0\r\nHost:starwars.com\r\n\r\n" | ncat localhost 7777