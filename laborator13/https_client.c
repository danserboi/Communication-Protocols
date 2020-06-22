#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>

#include "stdio.h"
#include "string.h"

int main()
{
    BIO * bio;
    SSL * ssl;
    SSL_CTX * ctx;

    int p;

    char * request = "GET / HTTP/1.0\r\nHost: www.verisign.com\r\n\r\n";
    char r[1024];

    /* initializare librarie */

    SSL_library_init ();
    ERR_load_BIO_strings();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    /* initializare context */
    ctx = SSL_CTX_new(SSLv23_client_method());    
   

    /* incarca trust store */


    if(! SSL_CTX_load_verify_locations(ctx, "TrustStore.pem", NULL))
    {
        fprintf(stderr, "Error loading trust store\n");
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return 0;
    }


    /* stabileste conexiune */

    bio = BIO_new_ssl_connect(ctx);
   
    /* Seteaza flag SSL_MODE_AUTO_RETRY  */

    BIO_get_ssl(bio, & ssl);
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

    /* setup conexiune */
    BIO_set_conn_hostname(bio, "verisign.com:443");
    if (BIO_do_connect(bio) <= 0)
    {
        fprintf(stderr, "Connection error\n");
        ERR_print_errors_fp(stderr);
        BIO_free_all(bio);
        SSL_CTX_free(ctx);
        return 0;
    }
    /* verifica certificat */
    BIO_get_ssl(bio, &ssl);
    if (SSL_get_verify_result(ssl) != X509_V_OK)
    {
        fprintf(stderr, "Certificate error\n");
        ERR_print_errors_fp(stderr);
        BIO_free_all(bio);
        SSL_CTX_free(ctx);
        return 0;
    }    


    /* Trimite request */

    BIO_write(bio, request, strlen(request));

    /* Citeste raspuns si pregateste  output*/
    int rr = BIO_read(bio, r, 1024);
    while(rr > 0)
    {
        printf("%s", r);
        rr = BIO_read(bio, r, 1024);
    }
    /* Inchide conexiune si elibereaza context */

    BIO_free_all(bio);
    SSL_CTX_free(ctx);
    return 0;
}
