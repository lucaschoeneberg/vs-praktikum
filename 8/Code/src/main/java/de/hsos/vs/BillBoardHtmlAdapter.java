package de.hsos.vs;

/**
 * Implementierung HTML BillBoard Adapters.
 * 
 * @author heikerli
 */
public class BillBoardHtmlAdapter extends BillBoard implements BillBoardAdapterIf {
    
    public BillBoardHtmlAdapter(String ctxt) {
        super (ctxt);
    };

    /**
     * Lesen eines Eintraeges. 
     *
     * @param idx Index des Parameters
     * @param caller_ip IP-Adresse des Aufrufers
     * @return Eintrag als Tabellen-Eintrag
     */
    @Override
    public String readEntry(int idx, String caller_ip) {
        BillBoardEntry bbe = getEntry(idx);
        if (bbe == null) {
            System.err.println("BillBoardServer - readEntry: Objekt null; ggf. ist Idx falsch");
            return null;
        }
        StringBuilder result = new StringBuilder();
        result.append("<tr><td>").append(bbe.id).append("</td>\n");
        result.append("<td>");
        String disable_edits = "";

        if (!bbe.belongsToCaller(caller_ip)) {
            disable_edits = " style=\"background-color: #eeeeee;\" readonly";
        }
        result.append("<input type=\"text\" size=\"100\" "
                + "minlength=\"100\" "
                + "maxlength=\"100\" "
                + "id=\"input_field_" + bbe.id + "\" "
                + "value=\"" + bbe.text + "\"" + disable_edits + ">");
        result.append("</td>");
        result.append("<td>");
        if (bbe.belongsToCaller(caller_ip)) {
            result.append("<button onClick=\"putHttpRequest('"
                    + getCtxt() + "',"
                    + bbe.id + ")\">Update</button>");
        }
        result.append("</td>");
        result.append("<td>");
        if (bbe.belongsToCaller(caller_ip)) {
            result.append("<button onClick=\"deleteHttpRequest('"
                    + getCtxt() + "',"
                    + bbe.id + ")\">Delete</button>");
        }
        result.append("</td>");
        result.append("</tr>");
        return result.toString();
    }
    
    /**
     * Lesen eines Eintrags. Der Eintrag wird als html-Tabelle
     * zurückgegeben und kann ohne weiteres in ein html-Dokument
     * eingebunden werden.
     * 
     * @param caller_ip IP-Adresse des Aufrufers
     * @return Eintrag als html-Tabelle
     */
    @Override
    public String readEntries(String caller_ip) {
        StringBuilder result = new StringBuilder();
        result.append("<table border=\"1\" rules=\"none\" cellspacing=\"4\" cellpadding=\"5\">\n");
        for (BillBoardEntry e : billboard) {
            result.append(readEntry(e.id, caller_ip));
        }
        return result.toString();
    };    
}
