package gtu.cse.cse396.sdbelt;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.autoconfigure.domain.EntityScan;
import org.springframework.boot.context.properties.ConfigurationPropertiesScan;
import org.springframework.context.annotation.ComponentScan;
import org.springframework.data.jpa.repository.config.EnableJpaAuditing;
import org.springframework.data.jpa.repository.config.EnableJpaRepositories;

import lombok.RequiredArgsConstructor;

@SpringBootApplication
@ComponentScan("gtu.cse.cse396.sdbelt")
@EnableJpaRepositories(basePackages = "gtu.cse.cse396.sdbelt")
@EnableJpaAuditing
@EntityScan(basePackages = "gtu.cse.cse396.sdbelt")
@ConfigurationPropertiesScan("gtu.cse.cse396.sdbelt")
@RequiredArgsConstructor
public class SdbeltApplication {

	public static void main(String[] args) {
		SpringApplication.run(SdbeltApplication.class, args);
	}

}
