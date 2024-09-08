// Some copyright should be here...

using UnrealBuildTool;

public class JakubCableComponent : ModuleRules
{
	public JakubCableComponent(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {

			}
			);


		PrivateIncludePaths.Add("CableComponent/Private");

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
					"Core",
					"CoreUObject",
					"Engine",
					"RenderCore",
					"RHI"
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"PhysicsCore",
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{

			}
			);
	}
}
