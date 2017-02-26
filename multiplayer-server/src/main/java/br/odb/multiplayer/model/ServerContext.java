package br.odb.multiplayer.model;

import javax.servlet.ServletContext;
import java.util.HashMap;

public class ServerContext {
    public final HashMap<Integer, Game> games = new HashMap<Integer, Game>();
    public final HashMap<String, Game> gameBuilders = new HashMap<String, Game>();

    public ServerContext() {
    }

    public static ServerContext createOrRetrieve(ServletContext servletContext) {

        ServerContext context = (ServerContext) servletContext
                .getAttribute("games-context");

        if (context == null) {
            reset(servletContext);

            context = (ServerContext) servletContext.getAttribute("games-context");
        }

        return context;
    }

    public static void reset(ServletContext servletContext) {
        servletContext.setAttribute("games-context", new ServerContext());
    }
}
