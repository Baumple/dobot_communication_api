gcc src/*.c -o dobot_com -Wall -Wextra -D RUN_TESTS

valgrind --leak-check=yes -s ./dobot_com
