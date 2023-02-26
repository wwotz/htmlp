#define HTMLP_IMPLEMENTATION
#include "htmlp.h"

int main(int argc, char **argv)
{
        htmlp_init(htmlp_info_create(HTMLP_FILE, "index.html"));
        if (htmlp_had_error()) {
                fprintf(stderr ,"%s\n", htmlp_debug_stack_pop());
                exit(EXIT_FAILURE);
        }

        htmlp_token tok;
        while ((tok = htmlp_get_token()).type != HTMLP_TYPE_EOF && tok.type != HTMLP_TYPE_ERROR) {
                if (tok.type == HTMLP_TYPE_OPEN_TAG) {
                        printf("HTMLP_TYPE_OPEN_TAG: %s\n", tok.token.data);
                } else if (tok.type == HTMLP_TYPE_CLOSE_TAG) {
                        printf("HTMLP_TYPE_CLOSE_TAG: %s\n", tok.token.data);
                } else if (tok.type == HTMLP_TYPE_STRING) {
                        printf("HTMLP_TYPE_STRING: %s\n", tok.token.data);
                }
        }
        return 0;
}
