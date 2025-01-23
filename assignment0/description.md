# COM S 327, Spring 2024
## Programming Project 0: Word Search Generator

You are to write a word search generator in C. Your word search generator will take an unbounded number of parameters on the command line. The first parameter will be a positive integer of magnitude 20 or smaller, giving the number of rows and columns in your desired word search (all generated word search puzzles will be square). All remaining parameters are words to be hidden in your puzzle. We will not require you to error check your parameters; your program may assume that all parameters are well formed (the integer is positive and smaller than 21 and all of the words are composed of letters and are short enough to fit in the puzzle).

Your words should be inserted randomly into valid locations in your puzzle (if you always start at the upper left corner, you’re going to generate some pretty boring word search puzzles!). Words may be hidden in four directions: diagonal up, rightward, diagonal down, and downward. If, after attempting to insert a word in all possible positions and locations, no valid location is found, your generator should terminate with a message about which word cannot be fit.

After all words are inserted, fill any empty positions in the puzzle with random letters.
When generation is complete, print your puzzle and word list.

The instructors will cover all of the features of C that you need to solve this problem in the first two lectures. In particular, we will cover how to process the command line, how to allocate static arrays and 2D arrays, how to recognize the end of a string, and how to generate random numbers. Note that the size limit on the puzzle tableau ensures that you will not require dynamic storage for this program! You’ll statically allocate a 20 × 20 matrix and use only that portion of it that your bounds require.