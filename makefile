# Makefile ou makefile est un fichier qui permet des exécutions, 
# telle la compilation d'un projet
# C'est un ensemble de règles, où chaque règle est de la forme:
 
#      cible: dépendances
#   		commandes
 
# Notons que chaque commande doit être précédée d'une tabulation

# dans Makefile, il existe plusieurs variables internes telles
# $@ Pour le nom de la cible
# $< Pour le nom de la première dépendance 
# $ˆ Pour la liste des dépendances 
# $? Pour la liste des dépendances plus récentes que la cible 
# $* Pour le nom du fichier, mais sans son suffixe.

FOL = build
CFOL = src
EXEC = exec
sudoku = sudoku
CC = gcc
CFLAGS = -O3 #Option d'optimisation du programme
FLAGS = -lm -lpthread -Wall
OBJ= $(FOL)/main.o $(FOL)/hashtable.o $(FOL)/tree.o $(FOL)/dynarr.o $(FOL)/chainedlist.o $(FOL)/misc.o $(FOL)/vector.o # $(FOL)/graph.o


all : $(FOL) $(FOL)/$(EXEC)
	./$(FOL)/$(EXEC)

$(FOL)/$(EXEC) : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(FLAGS)

$(FOL)/%.o : $(CFOL)/%.c
	$(CC) $(CFLAGS) -o $@ -c $< $(FLAGS)

$(FOL) :
	mkdir $(FOL)

rmproper :
	rm -r $(FOL)

sudoku: $(FOL) $(FOL)/$(sudoku)
	./$(FOL)/$(sudoku)

$(FOL)/$(sudoku) : $(CFOL)/$(sudoku).c
	$(CC) -o $@ $^ $(FLAGS)
