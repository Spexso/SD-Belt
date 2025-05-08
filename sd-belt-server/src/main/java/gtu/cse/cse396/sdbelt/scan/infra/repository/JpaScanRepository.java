package gtu.cse.cse396.sdbelt.scan.infra.repository;

import java.time.LocalDateTime;
import java.util.List;
import java.util.UUID;

import org.springframework.cglib.core.Local;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.JpaSpecificationExecutor;

import gtu.cse.cse396.sdbelt.scan.infra.model.ScanEntity;

public interface JpaScanRepository extends JpaRepository<ScanEntity, Long>, JpaSpecificationExecutor<ScanEntity> {

    // Custom query methods can be defined here if needed
    // For example, to find scans by productId or timestamp range
    List<ScanEntity> findByProductIdAndTimestampBetween(UUID productId, LocalDateTime startTime, LocalDateTime endTime);

    List<ScanEntity> findByProductId(UUID productId);

    List<ScanEntity> findByTimestampBetween(LocalDateTime startTime, LocalDateTime endTime);
}
