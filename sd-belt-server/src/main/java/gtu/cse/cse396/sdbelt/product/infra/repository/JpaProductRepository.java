package gtu.cse.cse396.sdbelt.product.infra.repository;

import java.util.UUID;
import org.springframework.data.jpa.repository.JpaRepository;

import gtu.cse.cse396.sdbelt.product.infra.model.ProductEntity;

public interface JpaProductRepository extends JpaRepository<ProductEntity, UUID> {
}
