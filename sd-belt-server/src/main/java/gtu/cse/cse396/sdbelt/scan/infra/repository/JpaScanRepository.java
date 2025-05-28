package gtu.cse.cse396.sdbelt.scan.infra.repository;

import java.time.LocalDateTime;
import java.util.List;

import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.JpaSpecificationExecutor;
import org.springframework.data.jpa.repository.Query;

import gtu.cse.cse396.sdbelt.scan.infra.model.ScanEntity;

public interface JpaScanRepository extends JpaRepository<ScanEntity, Long>, JpaSpecificationExecutor<ScanEntity> {

    @Query("SELECT s FROM ScanEntity s WHERE s.productId = :productId AND s.timestamp BETWEEN :startTime AND :endTime ORDER BY s.timestamp DESC")
    List<ScanEntity> findByProductIdAndTimestampBetween(String productId, LocalDateTime startTime,
            LocalDateTime endTime);

    @Query("SELECT s FROM ScanEntity s WHERE s.productId = :productId ORDER BY s.timestamp DESC")
    List<ScanEntity> findByProductId(String productId);

    @Query("SELECT s FROM ScanEntity s WHERE s.timestamp BETWEEN :startTime AND :endTime ORDER BY s.timestamp DESC")
    List<ScanEntity> findByTimestampBetween(LocalDateTime startTime, LocalDateTime endTime);
}
