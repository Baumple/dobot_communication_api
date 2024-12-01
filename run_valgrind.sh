gcc src/*.c -o dobot_com -Wall -Wextra -g
valgrind --leak-check=yes -s ./dobot_com
