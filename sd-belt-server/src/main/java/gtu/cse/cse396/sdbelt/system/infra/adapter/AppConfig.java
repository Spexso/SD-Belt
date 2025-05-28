package gtu.cse.cse396.sdbelt.system.infra.adapter;

import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.http.client.SimpleClientHttpRequestFactory;
import org.springframework.web.client.RestTemplate;

@Configuration
public class AppConfig {

    @Bean
    public RestTemplate restTemplate() {
        SimpleClientHttpRequestFactory factory = new SimpleClientHttpRequestFactory();
        factory.setConnectTimeout(500); // 2 seconds to establish the connection
        factory.setReadTimeout(1000); // 2 seconds to wait for data

        return new RestTemplate(factory);
    }
}
