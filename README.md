A fast C++ based wordle solver via an efficient bit-mask-based lexicographical index and a minimax algorithm. As a byproduct, the antagonistic portion of the solver can be played against in a version of wordle ("mean wordle") such that the "answer" word is always the most difficult valid word to deterministically guess. 

Mean wordle will walk you into scenarios where you have guessed something like S _ A R K, with possible words remaining "snark", "shark", "stark", "spark", etc., where it's impossible to deterministically find the correct word in fewer than the number of remaining words.

It can still be beaten in <=5 guesses, proving that wordle is always solvable given full knowledge of the dictionary.
