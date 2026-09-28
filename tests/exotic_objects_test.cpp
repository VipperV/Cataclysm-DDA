#include <set>
#include <string>
#include <vector>

#include "avatar.h"
#include "bodypart.h"
#include "calendar.h"
#include "cata_catch.h"
#include "coordinates.h"
#include "flag.h"
#include "item.h"
#include "item_category.h"
#include "item_factory.h"
#include "item_group.h"
#include "itype.h"
#include "iuse.h"
#include "npc.h"
#include "pocket_type.h"
#include "player_helpers.h"
#include "ret_val.h"
#include "type_id.h"
#include "units.h"

TEST_CASE( "portable_devices_reject_non_player_use", "[iuse][exotic_objects]" )
{
    item device;
    const tripoint_bub_ms pos;
    CHECK_FALSE( iuse::pocket_nanofab( nullptr, &device, pos ).has_value() );
    CHECK_FALSE( iuse::portable_autodoc( nullptr, &device, pos ).has_value() );
    npc non_player;
    CHECK_FALSE( iuse::pocket_nanofab( &non_player, &device, pos ).has_value() );
    CHECK_FALSE( iuse::portable_autodoc( &non_player, &device, pos ).has_value() );
}

TEST_CASE( "exotic_enhancement_preserves_healing_and_cardio", "[exotic_objects][mutations]" )
{
    const trait_id enhancement( "PROTOTYPE_GENETIC_ENHANCEMENT_HP" );
    if( !enhancement.is_valid() ) {
        SUCCEED( "Run with --mods=exotic_objects to exercise mod data." );
        return;
    }
    avatar patient;
    clear_character( patient );
    patient.set_lifestyle( 0 );
    const float base_healing = patient.healing_rate( 1.0f );
    const int base_cardio = patient.get_cardiofit();
    const int base_hp = patient.get_part_hp_max( bodypart_id( "torso" ) );
    REQUIRE( base_healing > 0 );

    patient.toggle_trait( enhancement );
    patient.recalculate_enchantment_cache();
    patient.recalc_hp();

    CHECK( patient.get_part_hp_max( bodypart_id( "torso" ) ) == base_hp + 500 );
    CHECK( patient.healing_rate( 1.0f ) == Approx( base_healing * 5 ) );
    CHECK( patient.healing_rate( 0.0f ) == Approx( base_healing * 20 ) );
    CHECK( patient.get_cardiofit() == base_cardio * 2 );
}

TEST_CASE( "exotic_storage_and_templates", "[exotic_objects]" )
{
    const itype_id ring_id( "dimensional_storage_ring" );
    if( !ring_id.is_valid() ) {
        SUCCEED( "Run with --mods=exotic_objects to exercise mod data." );
        return;
    }

    item ring( ring_id, calendar::turn );
    CHECK( ring.type->get_use( "PORTABLE_NANOFABRICATOR" ) != nullptr );
    CHECK( ring.type->get_use( "PORTABLE_AUTODOC" ) != nullptr );

    SECTION( "pocket_contents_do_not_add_weight_or_external_volume" ) {
        const units::mass empty_weight = ring.weight();
        const units::volume empty_volume = ring.volume();
        REQUIRE( ring.put_in( item( itype_id( "rock" ) ), pocket_type::CONTAINER ).success() );
        CHECK( ring.weight() == empty_weight );
        CHECK( ring.volume() == empty_volume );
    }

    SECTION( "templates_point_to_valid_recipes_after_migration" ) {
        REQUIRE_FALSE( ring.type->allowed_pocketnanofab_template_id.empty() );
        for( const itype_id &id : ring.type->allowed_pocketnanofab_template_id ) {
            INFO( id.str() );
            REQUIRE( id.is_valid() );
            REQUIRE( id->template_requirements.is_valid() );
            const std::vector<const itype *> recipes = id->nanofab_recipes();
            REQUIRE_FALSE( recipes.empty() );
            for( const itype *recipe : recipes ) {
                INFO( recipe->get_id().str() );
                CHECK_FALSE( recipe->get_id().is_null() );
                CHECK( recipe->get_id().is_valid() );
            }
            const item chip( id, calendar::turn );
            CHECK( itype_id( chip.get_var( "NANOFAB_ITEM_ID" ) ).is_valid() );
        }
    }

    SECTION( "self_replication_produces_an_advanced_template" ) {
        const item chip( itype_id( "advanced_template_construct_self" ), calendar::turn );
        CHECK( chip.get_var( "NANOFAB_ITEM_ID" ) == "advanced_template_construct" );
    }
}

TEST_CASE( "advanced_nanofab_uses_the_loaded_world_catalog", "[exotic_objects]" )
{
    const itype_id advanced( "advanced_template_construct" );
    if( !advanced.is_valid() ) {
        SUCCEED( "Run with --mods=exotic_objects to exercise mod data." );
        return;
    }
    REQUIRE( advanced->nanofab_template_all_items );
    const std::vector<const itype *> recipes = advanced->nanofab_recipes();
    std::set<itype_id> ids;
    for( const itype *recipe : recipes ) {
        INFO( recipe->get_id().str() );
        REQUIRE_FALSE( recipe->get_id().is_null() );
        REQUIRE( recipe->get_id().is_valid() );
        CHECK( ids.insert( recipe->get_id() ).second );
        // The selection UI constructs candidates to obtain their categories.
        const item candidate( recipe, calendar::turn );
        CHECK( candidate.get_category_shallow().get_id().is_valid() );
    }
    for( const itype *loaded : item_controller->all() ) {
        if( !loaded->get_id().is_null() ) {
            INFO( loaded->get_id().str() );
            CHECK( ids.count( loaded->get_id() ) == 1 );
        }
    }
    CHECK( ids.count( itype_id( "rock" ) ) == 1 );
    CHECK( ids.count( itype_id( "dimensional_storage_ring" ) ) == 1 );
    // Run with and without Exoplanet: no extension or recipe-list entry is needed.
    const itype_id exoplanet_weapon( "afs_xm34" );
    CHECK( ids.count( exoplanet_weapon ) == ( exoplanet_weapon.is_valid() ? 1 : 0 ) );

    // Standard templates retain their original, restricted item-group recipes.
    const itype_id standard( "standard_template_construct" );
    CHECK_FALSE( standard->nanofab_template_all_items );
    const std::vector<const itype *> standard_recipes = standard->nanofab_recipes();
    const std::set<const itype *> standard_set( standard_recipes.begin(), standard_recipes.end() );
    CHECK( standard_set == item_group::every_possible_item_from( standard->nanofab_template_group ) );

    // Creating random templates must not recursively construct template candidates.
    for( int i = 0; i < 100; ++i ) {
        const item chip( advanced, calendar::turn );
        CHECK( ids.count( itype_id( chip.get_var( "NANOFAB_ITEM_ID" ) ) ) == 1 );
    }
}
