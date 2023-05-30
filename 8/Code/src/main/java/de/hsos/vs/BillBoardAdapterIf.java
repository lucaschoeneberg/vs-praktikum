package de.hsos.vs;

import jakarta.json.JsonObject;
import org.json.JSONObject;

import java.util.Map;

/**
 * Interface eines Billboard Adapters.
 * Damit können verschiedene Implementierungen
 * zum Datenaustausch realisiert werden.
 *
 * @author heikerli
 */
public interface BillBoardAdapterIf {
    public String readEntries(String caller_ip);

    public JSONObject readEntriesJSON(String caller_ip);

    public String readEntry(int idx, String caller_ip);
}
