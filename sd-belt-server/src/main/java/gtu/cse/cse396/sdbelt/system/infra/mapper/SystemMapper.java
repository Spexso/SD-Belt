package gtu.cse.cse396.sdbelt.system.infra.mapper;

import org.springframework.stereotype.Component;
import gtu.cse.cse396.sdbelt.system.domain.model.System;
import gtu.cse.cse396.sdbelt.system.infra.model.SystemEntity;

@Component
public class SystemMapper {

    private SystemMapper() {
        // Private constructor to prevent instantiation
    }
    
    public static System toDomain(SystemEntity entity) {
        return new System(
                entity.getId(),
                entity.getName(),
                entity.getDescription(),
                entity.getCreatedAt(),
                entity.getRunAt(),
                entity.getStatus(),
                entity.getAccuracy(),
                entity.getSpeed()
        );
    }

    public static SystemEntity toEntity(System system) {
        return new SystemEntity(
                system.id(),
                system.name(),
                system.description(),
                system.createdAt(),
                system.runAt(),
                system.status(),
                system.accuracy(),
                system.speed()
        );
    }
  
} 
