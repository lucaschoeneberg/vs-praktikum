package de.hsos.vs;

import java.io.IOException;
import java.io.PrintWriter;
import java.util.Arrays;
import java.util.List;
import java.util.Objects;
import java.util.stream.Collectors;
import java.util.zip.GZIPOutputStream;

import jakarta.json.Json;
import jakarta.json.JsonArray;
import jakarta.json.JsonObject;
import jakarta.json.JsonReader;
import jakarta.servlet.ServletException;
import jakarta.servlet.annotation.WebServlet;
import jakarta.servlet.http.HttpServlet;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;
import org.json.JSONObject;

/**
 * Implementierung des BillBoard-Servers.
 * In dieser Version unterstützt er asynchrone Aufrufe.
 * Damit wird die Implementierung von Long Polling möglich:
 * Anfragen (HTTP GET) werden nicht sofort wie bei zyklischem
 * Polling beantwortet, sondern verbleiben so lange im System,
 * bis eine Änderung an den Client gemeldet werden kann.
 *
 * @author heikerli
 */
@WebServlet(asyncSupported = true, urlPatterns = {"/BillBoardServer/*"})
public class BillBoardServlet extends HttpServlet {
    private final BillBoardHtmlAdapter bb = new BillBoardHtmlAdapter("BillBoardServer");


    private final List<String> allowedIPs = Arrays.asList("*", "192.168.1.*", "192.168.0.*", "0:0:0:0:0:0:0", "0.0.0.0", "127.0.0.1");

    // <editor-fold defaultstate="collapsed" desc="HttpServlet methods. Click on the + sign on the left to edit the code.">

    /**
     * Handles the HTTP <code>GET</code> method.
     *
     * @param request  servlet request
     * @param response servlet response
     * @throws ServletException if a servlet-specific error occurs
     * @throws IOException      if an I/O error occurs
     */
    @Override
    protected void doGet(HttpServletRequest request, HttpServletResponse response)
            throws ServletException, IOException {
        String caller_ip = request.getRemoteAddr();

        /* Prüfung, ob der Aufrufer die nötigen Rechte hat */
        if (isAllowed(caller_ip)) {
            System.out.println("BillBoardServer - GET (" + caller_ip + "): forbidden");
            response.sendError(HttpServletResponse.SC_FORBIDDEN);
            return;
        }

        /* Ausgabe des gesamten Boards */
        System.out.println("BillBoardServer - GET (" + caller_ip + "): full output");
        if (Objects.equals(request.getContentType(), "application/json")) {
            response.setContentType("application/json;charset=UTF-8");
            response.setHeader("Content-Encoding", "gzip"); // set header for gzip
            try (GZIPOutputStream gzip = new GZIPOutputStream(response.getOutputStream());
                 PrintWriter out = new PrintWriter(gzip)) {
                JSONObject json = bb.readEntriesJSON(caller_ip);
                out.println(json);
                response.setStatus(HttpServletResponse.SC_OK);
            }
        } else {
            response.setContentType("text/html;charset=UTF-8");
            PrintWriter out = response.getWriter();
            out.println(bb.readEntries(caller_ip));
            response.setStatus(HttpServletResponse.SC_OK);
        }
    }

    /**
     * Handles the HTTP <code>POST</code> method.
     *
     * @param request  servlet request
     * @param response servlet response
     * @throws ServletException if a servlet-specific error occurs
     * @throws IOException      if an I/O error occurs
     */
    @Override
    protected void doPost(HttpServletRequest request, HttpServletResponse response)
            throws ServletException, IOException {
        String caller_ip = request.getRemoteAddr();
        System.out.println(request);

        /* Prüfung, ob der Aufrufer die nötigen Rechte hat */
        if (isAllowed(caller_ip)) {
            System.out.println("BillBoardServer - POST (" + caller_ip + "): forbidden");
            response.sendError(HttpServletResponse.SC_FORBIDDEN);
            return;
        }

        if (request.getContentType().equals("application/json")) {
            String body;
            try (JsonReader jsonReader = Json.createReader(request.getReader())) {
                JsonObject jsonObject = jsonReader.readObject();
                body = jsonObject.toString();
            }
            bb.createEntry(body, caller_ip);
            response.setStatus(HttpServletResponse.SC_CREATED);
            response.getWriter().close();
            return;
        }

        String body = request.getReader().lines().collect(Collectors.joining(System.lineSeparator()));
        System.out.println("BillBoardServer - POST (" + caller_ip + "): " + body);
        bb.createEntry(body, caller_ip);
        response.setStatus(HttpServletResponse.SC_CREATED);
        response.getWriter().close();
    }

    /**
     * Handles the HTTP <code>DELETE</code> method.
     *
     * @param request  servlet request
     * @param response servlet response
     * @throws ServletException if a servlet-specific error occurs
     * @throws IOException      if an I/O error occurs
     */
    @Override
    protected void doDelete(HttpServletRequest request, HttpServletResponse response)
            throws ServletException, IOException {
        String caller_ip = request.getRemoteAddr();

        /* Prüfung, ob der Aufrufer die nötigen Rechte hat */
        if (isAllowed(caller_ip)) {
            System.out.println("BillBoardServer - DELETE (" + caller_ip + "): forbidden");
            response.sendError(HttpServletResponse.SC_FORBIDDEN);
            return;
        }

        System.out.println("BillBoardServer - DELETE (" + caller_ip + ")");
        String id = request.getPathInfo();
        if (id == null || id.equals("/")) {
            response.sendError(HttpServletResponse.SC_BAD_REQUEST);
            return;
        }
        if (id.startsWith("/")) id = id.substring(1);
        bb.deleteEntry(Integer.parseInt(id));
        response.setStatus(HttpServletResponse.SC_NO_CONTENT);
        response.getWriter().close();
    }

    /**
     * Handles the HTTP <code>PUT</code> method.
     *
     * @param request  servlet request
     * @param response servlet response
     * @throws ServletException if a servlet-specific error occurs
     * @throws IOException      if an I/O error occurs
     */
    @Override
    protected void doPut(HttpServletRequest request, HttpServletResponse response)
            throws ServletException, IOException {
        String caller_ip = request.getRemoteAddr();

        System.out.println("BillBoardServer - PUT (" + caller_ip + ")");
        System.out.println("BillBoardServer - PUT (" + request.getRemoteHost() + ")");

        /* Prüfung, ob der Aufrufer die nötigen Rechte hat */
        if (isAllowed(caller_ip)) {
            System.out.printf("BillBoardServer - PUT (%s): forbidden%n", caller_ip);
            response.sendError(HttpServletResponse.SC_FORBIDDEN);
            return;
        }

        System.out.println("BillBoardServer - PUT (" + caller_ip + ")");
        String id = request.getPathInfo();
        if (id == null || id.equals("/")) {
            response.sendError(HttpServletResponse.SC_BAD_REQUEST);
            return;
        }
        if (id.startsWith("/")) id = id.substring(1);

        if (request.getContentType().equals("application/json")) {
            String body;
            try (JsonReader jsonReader = Json.createReader(request.getReader())) {
                JsonObject jsonObject = jsonReader.readObject();
                body = jsonObject.toString();
            }
            bb.updateEntry(Integer.parseInt(id), body, caller_ip);
            response.setStatus(HttpServletResponse.SC_NO_CONTENT);
            response.getWriter().close();
            return;
        }

        String body = request.getReader().lines().collect(Collectors.joining(System.lineSeparator()));
        System.out.println("BillBoardServer - PUT (" + caller_ip + "): " + body);
        bb.updateEntry(Integer.parseInt(id), body, caller_ip);
        response.setStatus(HttpServletResponse.SC_NO_CONTENT);
        response.getWriter().close();
    }

    /**
     * Returns a short description of the servlet.
     *
     * @return a String containing servlet description
     */
    @Override
    public String getServletInfo() {
        return "BillBoard Servlet";
    }// </editor-fold>

    /**
     * Liste der erlaubten IP-Adressen.
     *
     * @param ip
     * @return
     */
    private boolean isAllowed(String ip) {
        String[] parts = ip.split("\\."); // Teile die IP an den Punkten auf
        String subnet = String.join(".", parts[0], parts[1], parts[2]); // Bildet das Subnet (die ersten 3 Teile der IP)

        // Durchlaufe alle erlaubten IPs
        for (String allowedIP : allowedIPs) {
            if (allowedIP.equals("*") || allowedIP.contains(ip)) {
                // Wenn die IP ein Sternchen ist, ist alles erlaubt
                // Wenn die IP in der Liste enthalten ist, ist sie erlaubt
                return false;
            }

            // Wenn das letzte Element ein Sternchen ist, überprüfe das Subnet
            String[] allowedParts = allowedIP.split("\\.");
            if (allowedParts.length == 4 && allowedParts[3].equals("*")) {
                String allowedSubnet = String.join(".", allowedParts[0], allowedParts[1], allowedParts[2]);
                if (subnet.equals(allowedSubnet)) {
                    return false;
                }
            }
        }
        return true;
    }

}
