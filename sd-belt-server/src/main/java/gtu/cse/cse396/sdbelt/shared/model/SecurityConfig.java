package gtu.cse.cse396.sdbelt.shared.model;

import org.springframework.boot.web.servlet.FilterRegistrationBean;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

@Configuration
public class SecurityConfig {

    // Register our custom JWT filter
    @Bean
    public FilterRegistrationBean<JwtAuthFilter> tokenFilterRegistration() {
        FilterRegistrationBean<JwtAuthFilter> registrationBean = new FilterRegistrationBean<>();
        registrationBean.setFilter(new JwtAuthFilter());
        
        // Apply filter to all endpoints except login and logout
        registrationBean.addUrlPatterns("/api/*");
        registrationBean.addInitParameter("excludePatterns", "/api/v1/login,/api/v1/logout");
        
        // Set the order (lower value means higher priority)
        registrationBean.setOrder(1);
        
        return registrationBean;
    }
}
