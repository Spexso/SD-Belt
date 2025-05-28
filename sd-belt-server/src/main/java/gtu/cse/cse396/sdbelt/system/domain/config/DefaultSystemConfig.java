package gtu.cse.cse396.sdbelt.system.domain.config;

import java.util.UUID;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Component;

import lombok.Getter;

@Component
@Getter
public class DefaultSystemConfig {

    private final String name;
    private final String description;
    private final UUID id;
    private final Double threshold;
    private final Integer speed;

    public DefaultSystemConfig(
            @Value("${system.config.default.name}") String name,
            @Value("${system.config.default.description}") String description,
            @Value("${system.config.default.id}") UUID id,
            @Value("${system.config.default.threshold}") Double threshold,
            @Value("${system.config.default.speed}") Integer speed) {
        this.id = id;
        this.name = name;
        this.description = description;
        this.threshold = threshold;
        this.speed = speed;
    }
}
