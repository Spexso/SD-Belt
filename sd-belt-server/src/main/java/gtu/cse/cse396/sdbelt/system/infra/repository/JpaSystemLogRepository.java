package gtu.cse.cse396.sdbelt.system.infra.repository;

import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.JpaSpecificationExecutor;

import gtu.cse.cse396.sdbelt.system.infra.model.SystemStatusEntity;

public interface JpaSystemLogRepository
                extends JpaRepository<SystemStatusEntity, Long>, JpaSpecificationExecutor<SystemStatusEntity> {
}
