package br.odb.multiplayer;

import java.io.IOException;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import br.odb.multiplayer.model.ServerContext;

/**
 * Servlet implementation class ResetServer
 */
public class ResetServer extends HttpServlet {
    /**
	 * 
	 */
	private static final long serialVersionUID = 8259789304861242906L;

	/**
     * @see HttpServlet#HttpServlet()
     */
    public ResetServer() {
        super();
        // TODO Auto-generated constructor stub
    }

	/**
	 * @see HttpServlet#doGet(HttpServletRequest request, HttpServletResponse response)
	 */
	protected void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
		resetServer();
	}

	/**
	 * @see HttpServlet#doPost(HttpServletRequest request, HttpServletResponse response)
	 */
	protected void doPost(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
		resetServer();
	}
	
	public void resetServer() {
		ServerContext.reset( getServletContext() );
	}
}
