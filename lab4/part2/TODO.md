Improtant:

	- The current implementation has the following limitations:
		- It will save JPEG images in RGB of 255. No other maximum value
		  for the RGB is accepted
		- While it reads the stencil from the PGM format, it does not apply 
		  the transformation required for the lab. In that sense, it simply reads
		  the values from the file (we parse it directly).
		- The parallelization works, but it is not optimal. It automatically splits the images
		  in different chunks based on the number of threads, but does not take into account
		  anything else.


part 1b
read stencil properly