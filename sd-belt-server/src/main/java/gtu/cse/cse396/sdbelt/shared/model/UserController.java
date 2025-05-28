package gtu.cse.cse396.sdbelt.shared.model;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;


import java.util.HashMap;
import java.util.Map;

@RestController
@RequestMapping("/api/v1")
public class UserController {

    private static final String USERNAME = "erkanhocam";
    private static final String PASSWORD = "muhtesembirhoca";
    
    @Autowired
    private TokenService tokenService;
    
    @PostMapping("/login")
    public ResponseEntity<?> login(@RequestBody LoginRequest loginRequest) {
        // Check if credentials match
        System.out.println(loginRequest.getUsername());
        System.out.println(loginRequest.getPassword());
        if (USERNAME.equals(loginRequest.getUsername()) && 
            PASSWORD.equals(loginRequest.getPassword())) {
            
            // Generate a simple token using the service
            String token = tokenService.generateToken(loginRequest.getUsername());
            
            // Return token to the client
            Map<String, String> response = new HashMap<>();
            response.put("token", token);
            response.put("message", "Login successful");
            
            return ResponseEntity.ok(response);
        } else {
            Map<String, String> response = new HashMap<>();
            response.put("message", "Invalid credentials");
            return ResponseEntity.status(HttpStatus.UNAUTHORIZED).body(response);
        }
    }
    
    @PostMapping("/logout")
    public ResponseEntity<?> logout(@RequestHeader("Authorization") String token) {
        // Remove token from valid tokens
        if (token != null && token.startsWith("Bearer ")) {
            String actualToken = token.substring(7);
            tokenService.invalidateToken(actualToken);
        }
        
        Map<String, String> response = new HashMap<>();
        response.put("message", "Logged out successfully");
        return ResponseEntity.ok(response);
    }
    
    @GetMapping("/secure-resource")
    public ResponseEntity<?> getSecureResource() {
        // This endpoint is now protected by the filter
        // If we get here, the token is already validated
        Map<String, String> response = new HashMap<>();
        response.put("message", "This is a secure resource");
        response.put("data", "Your protected data here");
        
        return ResponseEntity.ok(response);
    }
    
    // Request class for login
    public static class LoginRequest {
        private String username;
        private String password;
        
        // Getters and setters
        public String getUsername() {
            return username;
        }
        
        public void setUsername(String username) {
            this.username = username;
        }
        
        public String getPassword() {
            return password;
        }
        
        public void setPassword(String password) {
            this.password = password;
        }
    }
}