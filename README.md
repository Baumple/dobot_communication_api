# References:
- Serial communicaton with c and gnu/linux: https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/
- Breakdown of the api: https://www.littlechip.co.nz/blog/communicating-with-the-dobot-magician-using-raw-protocol
- API Documentation: https://moodle-fbs.schulen-fulda.de/moodle/pluginfile.php/190626/mod_resource/content/1/Dobot-Communication-Protocol-V1.1.5-1.pdf

# TODO:
- [x] fix memory leak (free allocated param array) (checked with valgrind)
- [ ] use temp files instead of normal files for testing
