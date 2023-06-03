package de.hsos.vs;

import java.util.LinkedHashSet;
import java.util.Random;
import java.util.Set;

/**
 * Implementierung des Billboards.
 * Die Einträge werden in einer Liste vorgegebener Größe gespeichert.
 * Die Einträge vom Typ BillBoardEntry sind dabei vorformatiert.
 * Die Indizes freier Posterplätze werden dazu in einer Liste gespeichert.
 * Änderungen an Einträgen kann nur der Aufrufer vornehmen, der auch
 * den Eintrag erzeugt hat.
 *
 * @author heikerli
 */
public class BillBoard {
    private final short SIZE = 10;
    static private String servlet_ctxt;
    Set<Integer> freelist;
    public final BillBoardEntry[] billboard = new BillBoardEntry[SIZE];

    public BillBoard(String ctxt) {
        BillBoard.servlet_ctxt = ctxt;
        freelist = new LinkedHashSet<Integer>();
        for (int i = 0; i < SIZE; i++) {
            this.billboard[i] = new BillBoardEntry(i, "<empty>", "<not set>");
            freelist.add(i);
        }
    }

    ;

    /* Interne Funktion: Finden eines Slots zur Substitution durch neues Plakat */
    private int pickIndex() {
        Random rand = new Random();
        return rand.nextInt((SIZE - 1) + 1);
    }

    /* Liefert Ctxt des Servlets */
    public String getCtxt() {
        return servlet_ctxt;
    }

    /**
     * Eintrag zu Index zurueck geben.
     *
     * @param idx Index des Eintrages
     * @return BillBoard Eintrag
     */
    public BillBoardEntry getEntry(int idx) {
        return billboard[idx];
    }

    /**
     * Hinzufügen eines Eintrags.
     * Aus Gründen der Einfachheit und der Performanz
     * wird ein vorhandener Eintrag (vergleichbar einer
     * Plakatwand) überschrieben.
     *
     * @param text      Plakat-Text
     * @param poster_ip IP-Adresse des Senders
     * @return Index des neuen Eintrages
     */
    public int createEntry(String text, String poster_ip) {
        if (freelist.isEmpty()) {
            int picke_idx = pickIndex();
            deleteEntry(picke_idx);
        }
        /* Erstbestes Element auswählen */
        int idx = freelist.iterator().next();
        freelist.remove(idx);
        /* Inhalt überschreiben */
        assert (billboard[idx].id == idx) : "Indizierung nicht synchron!";
        billboard[idx].text = text;
        billboard[idx].owner_ip = poster_ip;
        billboard[idx].setTimeStamp();
        return idx;
    }

    /**
     * Löschen eines Eintrags via id.
     *
     * @param idx
     * @return Indext des Eintrages.
     */
    public int deleteEntry(int idx) {
        billboard[idx].reset();
        freelist.add(idx);
        return idx;
    }

    /**
     * Aktualisierung eines Eintrags.
     *
     * @param idx       id des Eintrags
     * @param text      Zu ersetzender Text
     * @param poster_ip IP-Adresse des Senders
     * @return Index des Eintrages
     */
    public int updateEntry(int idx, String text, String poster_ip) {
        assert (billboard[idx].owner_ip.equals(poster_ip)) : "Owner falsch!";
        billboard[idx].text = text;
        billboard[idx].setTimeStamp();
        return idx;
    }

    /**
     * Löschung aller Einträge.
     *
     * @return Anzahl der gelöschten Einträge
     */
    public int deleteAllEntries() {
        int count = 0;
        for (int i = 0; i < SIZE; i++) {
            if (!freelist.contains(i)) {
                deleteEntry(i);
                count++;
            }
        }
        return count;
    }
}
