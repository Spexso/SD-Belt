package gtu.cse.cse396.sdbelt.scan.infra.mapper;

import java.time.Instant;
import java.time.ZoneId;
import java.time.ZoneOffset;

import org.springframework.stereotype.Component;
import gtu.cse.cse396.sdbelt.scan.domain.model.Scan;
import gtu.cse.cse396.sdbelt.scan.infra.model.ScanEntity;

@Component
public class ScanMapper {

    private ScanMapper() {
        // Private constructor to prevent instantiation
    }
    
    public static Scan toDomain(ScanEntity entity) {
        return new Scan(
                entity.getProductId(),
                entity.getTimestamp()
                    .toInstant(ZoneOffset.ofHours(3))
                    .getEpochSecond(),
                entity.getIsSuccess(),
                entity.getErrorMessage()
        );
    }

    public static ScanEntity toEntity(Scan scan) {
        return new ScanEntity(
                null, // ID is auto-generated
                scan.productId(),
                Instant.ofEpochSecond(scan.timestamp())
                    .atZone(ZoneId.systemDefault())
                    .toLocalDateTime(),
                scan.isSuccess(),
                scan.errorMessage()
        );
    }
  
} 
