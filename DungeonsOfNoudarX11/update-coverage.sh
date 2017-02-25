#find . | grep gcno | xargs rm
#find . | grep gcda | xargs rm
#rm -rf coverage

lcov --capture --directory . --output-file lcov-output.info && genhtml lcov-output.info --output-directory coverage
rm lcov-output.info
