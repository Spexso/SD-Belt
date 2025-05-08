package gtu.cse.cse396.sdbelt.system.infra.repository;

import java.util.UUID;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.JpaSpecificationExecutor;

import gtu.cse.cse396.sdbelt.system.infra.model.SystemEntity;

public interface JpaSystemRepository extends JpaRepository<SystemEntity, UUID>, JpaSpecificationExecutor<SystemEntity> {
}
