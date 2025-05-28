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
                entity.getThreshold(),
                entity.getSpeed(),
                entity.getBeltDirection(),
                entity.getCpuUsage(),
                entity.getCpuTemperature(),
                entity.getMemoryUsage(),
                entity.getLastUpdated());
    }

    public static SystemEntity toEntity(System system) {
        return new SystemEntity(
                system.id(),
                system.name(),
                system.description(),
                system.createdAt(),
                system.runAt(),
                system.status(),
                system.threshold(),
                system.speed(),
                system.beltDirection(),
                system.cpuUsage(),
                system.cpuTemperature(),
                system.memoryUsage(),
                system.lastUpdated());
    }

}
