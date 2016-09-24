typedef struct{

	char *name;
	int len_name;
	char *type;

} Token;

Token get_token(FILE *file);