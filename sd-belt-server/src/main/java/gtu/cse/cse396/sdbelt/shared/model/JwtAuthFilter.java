package gtu.cse.cse396.sdbelt.shared.model;

import jakarta.servlet.Filter;
import jakarta.servlet.FilterChain;
import jakarta.servlet.FilterConfig;
import jakarta.servlet.ServletException;
import jakarta.servlet.ServletRequest;
import jakarta.servlet.ServletResponse;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Component;
import org.springframework.util.AntPathMatcher;

import java.io.IOException;
import java.util.Arrays;
import java.util.List;

@Component
public class JwtAuthFilter implements Filter {

    private List<String> excludePatterns;
    private AntPathMatcher pathMatcher = new AntPathMatcher();

    @Autowired(required = false)
    private TokenService tokenService;

    @Override
    public void init(FilterConfig filterConfig) {
        String excludePatternsString = filterConfig.getInitParameter("excludePatterns");
        if (excludePatternsString != null) {
            excludePatterns = Arrays.asList(excludePatternsString.split(","));
        }

        // If no TokenService is available, create a simple one
        if (tokenService == null) {
            tokenService = new TokenService();
        }
    }

    @Override
    public void doFilter(ServletRequest request, ServletResponse response, FilterChain chain)
            throws IOException, ServletException {

        HttpServletRequest httpRequest = (HttpServletRequest) request;
        HttpServletResponse httpResponse = (HttpServletResponse) response;

        String path = httpRequest.getRequestURI();

        System.out.println("PATH IS: " + path);

        // Skip authentication for excluded paths
        if (isPathExcluded(path)) {
            chain.doFilter(request, response);
            return;
        }

        // Check for authorization header
        String authHeader = httpRequest.getHeader("Authorization");

        System.out.println(authHeader);

        // Validate token
        if (authHeader != null && authHeader.startsWith("Bearer ")) {
            String token = authHeader.substring(7);
            if (tokenService.isValidToken(token)) {
                chain.doFilter(request, response);
                return;
            }
        }

        // Unauthorized if we get here
        httpResponse.setStatus(HttpStatus.UNAUTHORIZED.value());
    }

    private boolean isPathExcluded(String path) {

        if (path.equals("/api/v1/login") || path.equals("/websocket") ||
                path.equals("/api/v1/ws") ||
                path.equals("/h2-console") ||
                path.startsWith("/h2") ||
                path.startsWith("/") ||
                path.equals("/api/v1/logout")) {
            return true;
        }
        return false;

        /*
         * if (excludePatterns == null) {
         * return false;
         * }
         * 
         * for (String pattern : excludePatterns) {
         * if (pathMatcher.match(pattern.trim(), path)) {
         * return true;
         * }
         * }
         * 
         * return false;
         */
    }

    @Override
    public void destroy() {
        // Nothing to do here
    }
}