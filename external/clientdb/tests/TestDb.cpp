#include "test.h"
#include "ClientDb.h"

using namespace ClientDb;

struct MockViewDelegate
{
	MOCK_METHOD3(call, void(View::Event,size_t,size_t));
};

static function<void(const function<void()> &)> queuefn(Serializer &executor)
{
	return [&](const function<void()> &f){
		executor.addTask(f);
	};
}

TEST(Db, Add)
{
	using namespace testing;

	Serializer background;
	Serializer ui;


	Index index("DataItem", "", {"time"});
	std::vector<Index> indexes = {index};

	async([&](const function<void()> &done){

		Db::destroy("test.db");
		Db::open("test.db", indexes, queuefn(background), [done,&ui,index](shared_ptr<Db> db) {
			
			EXPECT_TRUE((bool)db);
			if (db) {
				MockViewDelegate delegate;

				// should be able to create a view
				auto delfn = [&delegate](const shared_ptr<View> &view, View::Event a,size_t b,size_t c){delegate.call(a,b,c);};
				auto view = db->view("DataItem", Predicate(), SortDescriptor({"time"},true),  queuefn(ui), delfn);

				// wrong:
				/*testing::InSequence seq;
				EXPECT_CALL(delegate, call(View::Event::Inserted,0,1));
				EXPECT_CALL(delegate, call(View::Event::Inserted,1,1));
				*/
				// ... because no initial fetch is triggered by size() yet
				//instead:

				EXPECT_CALL(delegate, call(View::Event::Inserted,0,1)).Times(2);

				Json::Value value;
				value["time"] = 1;
				//EXPECT_EQ("DataItem//1", index.key(value));
				db->upsert("DataItem", value);

				value["time"] = 2;
				//EXPECT_EQ("DataItem//2", index.key(value));
				db->upsert("DataItem", value);

				std::this_thread::sleep_for(milliseconds(20));

				testing::Mock::VerifyAndClearExpectations(&delegate);
				EXPECT_EQ(2, view->size());

				db.reset();

				done();
			}
		});
	});
	
	// re-open with the previous 2 items

	async([&](const function<void()> &done){
		Db::open("test.db", indexes, queuefn(background), [done,&ui,index](const shared_ptr<Db> db){
			
			EXPECT_TRUE((bool)db);
			if (db) {
				MockViewDelegate delegate;
				shared_ptr<View> view;
				/*EXPECT_CALL(delegate, call(View::Event::Inserted,0,2))
					.Times(1)
					.WillOnce(InvokeWithoutArgs([&]{EXPECT_EQ(2, view->size());}));
*/
				// should be able to create a view
				auto delfn = [&delegate](const shared_ptr<View> &, View::Event a,size_t b,size_t c){delegate.call(a,b,c);};
				view = db->view("DataItem", Predicate(), SortDescriptor({"time"},true), queuefn(ui), delfn);

				EXPECT_EQ(2, view->size());

				//std::this_thread::sleep_for(milliseconds(20));
				//EXPECT_EQ(2, view->size());

				//EXPECT_CALL(delegate, call(View::Event::Updated, 0, 2));
				//EXPECT_TRUE(view->at(1).isNull());
				std::this_thread::sleep_for(milliseconds(20));
				EXPECT_FALSE(view->at(1).isNull());
				

				done();
			}
		});
	});
}

/*
TEST(Db, Read)
{
	using namespace testing;

	SerialExecutor background;
	SerialExecutor ui;

	Index index1("DataItem", "", {"originalDate"});
	Index index2("DataItem", "ids", {"datasource","identifier"});
	std::vector<Index> indexes = {index1,index2};


	async([&](const function<void()> &done){

		// don't modify this db!
		Db::open("tests/db1.db", indexes, queuefn(background), [done,&ui](shared_ptr<Db> db){
			if (db) {
				NiceMock<MockViewDelegate> delegate;

				auto delfn = [&delegate](View::Event a,size_t b,size_t c){
					std::cout << "event: " << (int)a << " pos: " << b << " size: " << c << std::endl;
					delegate.call(a,b,c);
				};
				auto view = db->view("DataItem", {"originalDate"}, queuefn(ui), delfn);
				EXPECT_EQ(0, view->size());

				sleep_for(milliseconds(20));
				EXPECT_EQ(19, view->size());

				//EXPECT_TRUE(view->at(1).isNull());

				done();
			}
		}, shared_ptr<IBackend>(new LevelDbBackend()));
	});
}*/
