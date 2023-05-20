# ECS
An ECS I made.
## Context:
I come from a non professional Luau Roblox background.

I started programming in C for the first time around late July of 2022.

I had used many ECS libraries in roblox for around 1.5-ish years prior to starting C and was infatuated with them. It's a shame most of the ones I used were terribly complex. But a friend I met had showed me their simple lua ecs and I was astonished with how small it was. It cheated quite a bit on the performance front by essentially storing a pointer inside every component, but that made the ecs very small and easy to understand front to back. When I started in C, I wanted to recreate my friends ecs in C to better learn C. I completed it early early October of 2022. With a basic understanding of C I wanted to jump into making an ecs as good as I possibly could. I had used unity in the passed and kept up closely with their work on DOTS and knew about the concept of archetypal ecs's while working in roblox. An Archetypal ECS was the one I knew I needed to make.

I started writing this ECS sometime around October of 2022.

This is somewhere around the 20th rewrite.


Things to note when looking at this codebase:
I love C. I hate most programming culture. I intentionally ignored worrying about code smells and rough around the edges syntax. I hyperfocused on the language agnostic ideas in order to make the ecs work as plainly as I could. My reason for this, is all of the "proper" ways of writing high quality C code I could go back and do, I just don't care. Learning how to make and use the ECS as well as C's functionality was more important to me than code prettiness.

One last thing:
I love C, but it wasn't good enough for some key things I wanted. I wanted to be able to more trivially and easily use my ECS and C's macro functions while very powerful, were lacking in certain situations. So I wrote an entire metaprogramming preprocessor that transpiles portions of my non-vanilla C code into vanilla C. I wanted this for more than just to make the ECS easier to use, but for many reasons relating to C and it's age. The preprocessor's syntax is loosely off of openMP where you can type something along the lines of :
```c
#pragma omp parallel for
{
	//insert normal C forloop here
}
```
OpenMP makes use of custom pragmas that perform different actions in the following code block. Mine works almost exactly the same, except I was very lazy while writing the preprocessor, so a code block is instead define by the normal OpemMP-like pragma and then at the end of the block, another pragma is required. You could call it the closing pragma. If it is the end of that file, then the closing pragma is implied. Interestingly this makes my preprocessor somewhat more similar to the Fortran version of OpenMP than the C version. I thought that was neat.

Oh, also there are certain areas of the ecs that I simply haven't optimized yet/took the naïve solution for the sole reason that they are well contained and I will fix them later. I am currently developing a small 3D hide and seek game in order to test the ECS. During and/or after the development of this game, I will finish making the optimizations to all of the known points that need them. This project has a lot of comments scattered about, some of which act as reminders of these points that I must revisit.
