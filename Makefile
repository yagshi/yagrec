yagrec:	yagrec.cc
	clang -o $@ $< -lasound -lstdc++ -lsndfile

