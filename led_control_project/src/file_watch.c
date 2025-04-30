#include "file_watch.h"
// Fonction pour surveiller les modifications d'un fichier
int surveiller_fichier(const char *filename) {
    // Initialiser inotify en mode non bloquant
    int ifd = inotify_init1(IN_NONBLOCK);
    if (ifd == -1) {
        perror("inotify_init1");
        return -1;
    }

    // Ajouter une surveillance sur le fichier
    int wd = inotify_add_watch(ifd, filename, IN_MODIFY);
    if (wd == -1) {
        perror("inotify_add_watch");
        close(ifd);
        return -1;
    }

    char buff[sizeof(struct inotify_event) + NAME_MAX + 1];
    ssize_t len;

    printf("Surveillance du fichier : %s\n", filename);

    while (1) {
        len = read(ifd, buff, sizeof(buff));
        if (len == -1) {
            if (errno == EAGAIN) {
                // Aucune donnée disponible, attendre ou faire autre chose
                sleep(1); // Attendre un court moment avant de réessayer
                continue;
            } else {
                perror("read");
                inotify_rm_watch(ifd, wd);
                close(ifd);
                return -1;
            }
        } else if (len == 0) {
            // Fin de fichier ou aucune donnée lue
            break;
        } else {
            // Traiter les données lues depuis le descripteur de fichier inotify
            struct inotify_event *event = (struct inotify_event *)buff;
            if (event->len) {
                printf("Modification détectée dans le fichier : %s\n", event->name);
            }
        }
    }

    // Supprimer la surveillance
    inotify_rm_watch(ifd, wd);
    close(ifd);
    return 0;
}