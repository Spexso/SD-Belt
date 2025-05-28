package gtu.cse.cse396.sdbelt.shared.model;

import org.springframework.context.annotation.Scope;
import org.springframework.stereotype.Service;
import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

@Service
@Scope("application")
public class TokenService {
    
    // Simple in-memory token store
    private static final Map<String, String> validTokens = new HashMap<>();
    
    /**
     * Generate a new token for a user
     * 
     * @param username The username to generate token for
     * @return The generated token
     */
    public String generateToken(String username) {
        String token = UUID.randomUUID().toString();
        System.out.println("Putting token to hashmap: " + token);
        validTokens.put(token, username);
        return token;
    }
    
    /**
     * Validate if a token is valid
     * 
     * @param token The token to validate
     * @return true if valid, false otherwise
     */
    public boolean isValidToken(String token) {
        System.out.println("checking token validation" + token);
        System.out.println(validTokens.containsKey(token));
        return validTokens.containsKey(token);
    }
    
    /**
     * Invalidate a token (logout)
     * 
     * @param token The token to invalidate
     */
    public void invalidateToken(String token) {
        validTokens.remove(token);
    }
}
