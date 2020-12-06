run: main.cpp
	g++ main.cpp -o run -lglut -lGLU -lGL -lGLEW -lglfw -lfreeimageplus
clean:
	rm run