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
                entity.getHealthRatio(),
                entity.getIsSuccess(),
                entity.getErrorMessage(),
                entity.getTimestamp());
    }

    public static ScanEntity toEntity(Scan scan) {
        return new ScanEntity(
                null, // ID is auto-generated
                scan.productId(),
                scan.healthRatio(),
                scan.isSuccess(),
                scan.errorMessage(),
                scan.timestamp());
    }

}
